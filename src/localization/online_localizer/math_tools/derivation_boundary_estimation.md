# Derivation for boundary estimation between two weighted Gaussians

Assuming that were are given two Gaussian distributions $\mathcal{N}(\mu_1, \sigma_1)$ and $\mathcal{N}(\mu_2, \sigma_2)$ and weights $\pi_1$ and $\pi_2$ respectively.
Then, we find select the boundary at the point $x$ such that
$$
    \pi_1\mathcal{N}(x | \mu_1, \sigma_1) = \pi_2\mathcal{N}(x | \mu_2, \sigma_2)
$$

the point x has a equal probability to belong to two different distributions.

$$

\begin{align}
    \frac{\pi_1}{\sqrt{2\pi}\sigma_1} \exp{(-\frac{(x-\mu_1)^2}{2\sigma_1^2})} = \frac{\pi_2}{\sqrt{2\pi}\sigma_2} \exp{(-\frac{(x-\mu_2)^2}{2\sigma_2^2})}  && \text{(Multiple by ${\sqrt{2\pi}}$ and by $\ln$)} \\

    \ln(\frac{\pi_1}{\sigma_1}) - \frac{(x-\mu_1)^2}{2\sigma_1^2} = \ln(\frac{\pi_2}{\sigma_2}) - \frac{(x-\mu_2)^2}{2\sigma_2^2}  && \text{(Rearrage)} \\
     - \frac{(x-\mu_1)^2}{2\sigma_1^2} + \frac{(x-\mu_2)^2}{2\sigma_2^2} =  \ln(\frac{\pi_2}{\sigma_2}) - \ln(\frac{\pi_1}{\sigma_1}) && \text{(Open brackets)} \\

    - \frac{1}{2\sigma_1^2} \left( x^2 - 2x\mu_1 + \mu_1^2 \right)
    + \frac{1}{2\sigma_2^2} \left( x^2 - 2x\mu_2 + \mu_2^2 \right) = \ln\left(\frac{\pi_2}{\sigma_2}\right) - \ln\left(\frac{\pi_1}{\sigma_1}\right)  && \text{(Collect for x)} \\ 

    \left(  
        \frac{1}{2\sigma_2^2} - \frac{1}{2\sigma_1^2}
    \right) x^2 +
    \left(
        \frac{2\mu_1}{2\sigma_1^2} - \frac{2\mu_2}{2\sigma_2^2}
    \right) x +
    \frac{\mu_2^2}{2\sigma_2^2} - \frac{\mu_1^2}{2\sigma_1^2} =
    \ln \left( \frac{\pi_2}{\sigma_2}\right) - \ln \left( \frac{\pi_1}{\sigma_1}\right)

\end{align}
$$

Now the quadratic equation of form $Ax^2 + Bx + C = 0$ can be seen where:

$$
\begin{align}
    A = \frac{1}{2\sigma_2^2} - \frac{1}{2\sigma_1^2} \\
    B =  \frac{\mu_1}{\sigma_1^2} - \frac{\mu_2}{\sigma_2^2} \\
    C = \frac{\mu_2^2}{2\sigma_2^2} - \frac{\mu_1^2}{2\sigma_1^2} - \ln \left( \frac{\pi_2 \sigma_1}{\sigma_2 \pi_1} \right)
\end{align}
$$

When solving this quadratic equation, we may encounter the following situations, The discriminant $D = B^2 - 4AC$

1. D > 0 

    The equation has two real solutions. $x_1 = -B + \frac{\sqrt{D}}{2A}$ and $x_2 = -B - \frac{\sqrt{D}}{2A}$.
2. D = 0

    The equation has a unique soluion
    $x = \frac{-B}{2A}$.
3. D < 0

    The equation does not have real solutions.

For the cases, where we do not have a unique solution, case 1. We select the solution that lies between $\mu_1 < x < \mu_2$ if $\mu_2 > \mu_1$ and $\mu_2 < x < \mu_1$ otherwise. 


### Border condition

In case $\sigma_1 = \sigma_2$ the coefficient $A = 0$ and we have a linear equation with respect to $x$.

$$
\begin{align}
    \left(
        \frac{2\mu_1}{2\sigma^2} - \frac{2\mu_2}{2\sigma^2}
    \right) x +
    \frac{\mu_2^2}{2\sigma^2} - \frac{\mu_1^2}{2\sigma^2} =
    \ln \left( \frac{\pi_2}{\sigma}\right) - \ln \left( \frac{\pi_1}{\sigma}\right) \\

    \left(
        \frac{\mu_1 - \mu_2}{\sigma^2}
    \right) x +
    \frac{\mu_2^2 - \mu_1^2}{2\sigma^2} =
    \ln \left( \frac{\pi_2}{\pi_1}\right) \\

    x = \frac{2\sigma^2 \ln{\frac{\pi_2}{\pi_1}} - \left(\mu_2^2 - \mu_1^2 \right)}{2\left( \mu_1-\mu_2 \right)}

\end{align}
$$

That is it!