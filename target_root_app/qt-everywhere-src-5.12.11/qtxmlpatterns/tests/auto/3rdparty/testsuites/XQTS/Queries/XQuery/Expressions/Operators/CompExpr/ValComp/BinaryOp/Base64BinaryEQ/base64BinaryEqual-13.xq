(:Test: base64BinaryEqual-13                                   :)
(:Description: Simple operation involving xs:base64Binary values used as part of a   :)
(:logical expression.  Use "or" and "ne" operators and "fn:true" function.      :)

(xs:base64Binary("cmxjZ3R4c3JidnllcmVuZG91aWpsbXV5Z2NhamxpcmJkaWFhbmFob2VsYXVwZmJ1Z2dmanl2eHlzYmhheXFtZXR0anV2dG1q") ne xs:base64Binary("d2J1bnB0Y3lucWtvYXdpb2xoZWNwZXlkdG90eHB3ZXJqcnliZXFubmJjZXBmbGx3aGN3cmNndG9xb2hvdHdlY2pzZ3h5bnlp")) or fn:true()