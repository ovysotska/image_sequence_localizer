import numpy as np
import cv2
import argparse
from pathlib import Path
import protos_io


def createHiddenMatchImage(width, height):
    image = np.zeros((height, width, 3), dtype=np.uint8)
    text_left_corner = (int(width / 3), int(height / 2))
    image = cv2.putText(
        image,
        "No match found",  # text
        text_left_corner,
        cv2.FONT_HERSHEY_SIMPLEX,  # fontFace
        1,  # fontScale
        (255, 255, 255),  # color
        2,  # thickness
    )
    return image


def resize_image_by_height(image, target_height):
    original_height, original_width = image.shape[:2]
    aspect_ratio = original_width / original_height
    new_width = int(target_height * aspect_ratio)
    resized_image = cv2.resize(
        image, (new_width, target_height), interpolation=cv2.INTER_AREA
    )
    return resized_image


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--matching_result",
        required=True,
        type=Path,
        help="Path to the matching result .MatchingResult.pb file",
    )
    parser.add_argument(
        "--query_images",
        required=True,
        type=Path,
        help="Path to directory with query images.",
    )
    parser.add_argument(
        "--reference_images",
        required=True,
        type=Path,
        help="Path to directory with reference images.",
    )
    parser.add_argument(
        "--output_dir",
        required=True,
        type=Path,
        help="Output dir for the resulting images.",
    )

    args = parser.parse_args()

    matching_result = protos_io.read_matching_result(args.matching_result)

    if args.output_dir.exists():
        print(
            "WARNING: the output directory exists. Potentially rewritting the results."
        )

    args.output_dir.mkdir(exist_ok=True)

    query_images = list(args.query_images.glob("*.png"))
    query_images.extend(args.query_images.glob(".jpg"))
    query_images = sorted(query_images)

    reference_images = list(args.reference_images.glob("*.png"))
    reference_images.extend(args.reference_images.glob(".jpg"))
    reference_images = sorted(reference_images)

    for idx, match in enumerate(reversed(matching_result.matches)):
        query_image = cv2.imread(str(query_images[match.query_id]))
        if not match.real:
            reference_image = createHiddenMatchImage(
                query_image.shape[1], query_image.shape[0]
            )
        else:
            reference_image = cv2.imread(str(reference_images[match.ref_id]))
            reference_image = resize_image_by_height(
                reference_image, query_image.shape[0]
            )

        stacked = cv2.hconcat([query_image, reference_image])
        img_name = args.output_dir / f"match_{idx:05d}.jpg"
        # img_name = args.output_dir / "query:{qu_id}_ref:{ref_id}.jpg".format(
        # qu_id=match.query_id, ref_id=match.ref_id
        # )
        cv2.imwrite(str(img_name), stacked)
        print("Saved image", img_name)


if __name__ == "__main__":
    main()
