
MackensteinCAM
==============

MackensteinCAM

A camera-based system for automatic identification and localization of electrodes.

Developed by the Institute of Biomedical Engineering (IBT) of the Karlsruhe Institute of Technology (KIT).
This tool is distributed under the MIT License (MIT).

Features:
- localizes multiple electrode positions at once
- accuracy 1±1 mm
- system of markers for robust identification using an error-correcting code

Includes all algorithms from the following paper:
http://dx.doi.org/10.1515/bmt-2014-0018, which explains all processing steps and includes a validation.

The code requires

* the open source computer vision (OpenCV) library (version 2.4.6.0), which is distributed under a BSD license
* the Augmented Reality library from the University of Cordoba (ArUco) (version 1.1.0), which is distributed under a BSD licence

If you are interested in using MackensteinCAM and haven’t yet told us about it, please do so and send us an email: ws@evolunis.com. We are very happy to help you get started.

Are you using MackensteinCAM in research work? If so, please include explicit mention of our work in your related publications:

<pre>
@article{MackensteinCAM2014,
  title={Automatic camera-based identification and 3-D reconstruction of electrode positions in electrocardiographic imaging},
  author={Schulze, Walther HW and Mackens, Patrick and Potyagaylo, Danila and Rhode, Kawal and T{\"u}l{\"u}men, Erol and Schimpf, Rainer and Papavassiliu, Theano and Borggrefe, Martin and D{\"o}ssel, Olaf},
  journal={Biomedical Engineering/Biomedizinische Technik},
  volume={59},
  number={6},
  pages={515--528},
  year={2014}
}
</pre>
