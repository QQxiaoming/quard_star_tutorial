(:Test: hexBinary-13                                      :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "or" and "ne" operators and "fn:true" function.      :)

(xs:hexBinary("786174616d61616772") ne xs:hexBinary("767479716c6a647663")) or fn:true()