(:Test: hexBinary-4                                       :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "and" and "eq" operators with "fn:true" function.      :)

(xs:hexBinary("786174616d61616772") eq xs:hexBinary("767479716c6a647663")) and fn:true()