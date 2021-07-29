(:Test: hexBinary-11                                       :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "and" and "ne" operators with "fn:true" function.      :)

(xs:hexBinary("786174616d61616772") ne xs:hexBinary("767479716c6a647663")) and fn:true()