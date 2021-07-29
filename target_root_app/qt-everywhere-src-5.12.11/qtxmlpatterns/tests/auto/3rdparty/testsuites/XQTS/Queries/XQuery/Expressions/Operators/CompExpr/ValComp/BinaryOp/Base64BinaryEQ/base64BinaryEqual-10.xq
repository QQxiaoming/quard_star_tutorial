(:Test: base64BinaryEqual-10                                       :)
(:Description: Simple opeartion involving xs:base64Binary values as part of argument to "fn:not" function. :)

fn:not(xs:base64Binary("cmxjZ3R4c3JidnllcmVuZG91aWpsbXV5Z2NhamxpcmJkaWFhbmFob2VsYXVwZmJ1Z2dmanl2eHlzYmhheXFtZXR0anV2dG1q") ne xs:base64Binary("d2J1bnB0Y3lucWtvYXdpb2xoZWNwZXlkdG90eHB3ZXJqcnliZXFubmJjZXBmbGx3aGN3cmNndG9xb2hvdHdlY2pzZ3h5bnlp"))