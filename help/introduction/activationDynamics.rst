.. _introActivationDynamics:

Activation dynamics
===================

Muscle excitations are extracted from raw EMG signals using a standard filtering procedure. Raw EMG signals are first high-pass filtered using a zero-lag fourth-order recursive Butterworth filter (30 Hz), then full wave rectified, and finally filtered using a Butterworth low-pass filter with a 6 Hz cutoff frequency. This processed EMG signal is called the muscle excitation, :math:`e(t)`.
Neural activation is derived from muscle excitation by modelling the muscle’s twitch response in the activation dynamic model, which has been shown to improve muscle force predictions :cite:`buchanan2004neuromusculoskeletal,lloyd2003emg,lloyd2008neuromusculoskeletal`. This is represented by a critically damped linear second-order differential system46, expressed in a discrete form by using backward differences :cite:`buchanan2004neuromusculoskeletal,lloyd2003emg,lloyd2008neuromusculoskeletal`.

.. math::
   u_j(t) = \alpha e_j(t-d) - \beta_1 u_j(t-1) -\beta_2 u_j(t-2)


where :math:`e_j` is the :math:`j`-th muscle excitation at time :math:`t`, :math:`u_j` is the neural activation, :math:`\alpha` is the muscle gain coefficient, :math:`\beta_1` and :math:`\beta_2` are the recursive coefficients, and :math:`d` is the electromechanical delay. A stable solution for this is obtained including the following constrains :cite:`buchanan2004neuromusculoskeletal,lloyd2003emg`

.. math::

   \beta_1 = C_1 + C_2

   \beta_2 = C_1 \cdot C_2

where:


.. math::
  |C_1| < 1

.. math::
  |C_2| < 1

and

.. math::

   \alpha - \beta_1 - \beta_2 = 1

The relation between neural activation and the muscle activation is non-linear, and CEINMS has two different solutions :cite:`buchanan2004neuromusculoskeletal,lloyd2003emg,manal2003one`. The first was introduced by :cite:`lloyd2003emg`,

.. math::

   a_j(t)=\frac{e^{A_ju_j(t)}-1}{e^{A_j}-1}

where :math:`a_j(t)` is the activation of the :math:`j`-th muscle, and :math:`A_j` is the non-linear shape factor, constrained in the interval :math:`(-3, 0)`.

The second model was introduced and described by


. The :math:`u_j \leftarrow a_j` transformation is defined as a piecewise parametric function.

.. math::

   a_j(t) = \alpha_j^{act} ln(\beta_j^{act}u_j(t)+1), 0 \leq u_j(t)	\le u_0

   a_j(t) = m_j u_j(t) + c_j, u_0 \leq u_j(t) \leq 1

For each muscle :math:`j`, the parameters :math:`\alpha_j^{act}`,  :math:`\beta_j^{act}`, :math:`m_j`, :math:`c_j` depend only from the shape factor , constrained in the interval :math:`(0, 0.12]`.
