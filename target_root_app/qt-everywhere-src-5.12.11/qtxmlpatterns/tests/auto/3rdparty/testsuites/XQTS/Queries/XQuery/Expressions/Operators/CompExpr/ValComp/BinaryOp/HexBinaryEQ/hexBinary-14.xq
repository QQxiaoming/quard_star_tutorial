(:Test: hexBinary-14                                     :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "or" and "ne" operators and "fn:false" function.      :)

(xs:hexBinary("786174616d61616772") ne xs:hexBinary("767479716c6a647663")) or fn:false()