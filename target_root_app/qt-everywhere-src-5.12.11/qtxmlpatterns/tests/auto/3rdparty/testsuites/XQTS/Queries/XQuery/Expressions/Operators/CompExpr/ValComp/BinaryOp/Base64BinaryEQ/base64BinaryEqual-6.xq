(:Test: base64BinaryEqual-6                                       :)
(:Description: Simple operation using xs:base64Binary values as part of a   :)
(:logical expression.  Use "or" and "eq" operators and "fn:true" function.      :)

(xs:base64Binary("cmxjZ3R4c3JidnllcmVuZG91aWpsbXV5Z2NhamxpcmJkaWFhbmFob2VsYXVwZmJ1Z2dmanl2eHlzYmhheXFtZXR0anV2dG1q") eq xs:base64Binary("d2J1bnB0Y3lucWtvYXdpb2xoZWNwZXlkdG90eHB3ZXJqcnliZXFubmJjZXBmbGx3aGN3cmNndG9xb2hvdHdlY2pzZ3h5bnlp")) or fn:true()