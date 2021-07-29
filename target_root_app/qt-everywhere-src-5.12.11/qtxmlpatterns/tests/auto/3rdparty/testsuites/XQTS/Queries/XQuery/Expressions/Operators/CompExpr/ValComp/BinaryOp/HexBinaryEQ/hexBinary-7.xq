(:Test: hexBinary-7                                      :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "or" and "eq" operators and "fn:false" function.      :)

(xs:hexBinary("786174616d61616772") eq xs:hexBinary("767479716c6a647663")) or fn:false()