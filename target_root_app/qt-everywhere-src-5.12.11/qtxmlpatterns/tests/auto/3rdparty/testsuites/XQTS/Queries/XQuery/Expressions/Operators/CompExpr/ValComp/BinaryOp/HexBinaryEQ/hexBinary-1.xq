(:Test: hexBinary-1                                       :)
(:Description: Simple Binary hex operation as part of a   :)
(:logical expression.  Use "and" and "eq" operators.      :)

(xs:hexBinary("786174616d61616772") eq xs:hexBinary("767479716c6a647663")) and (xs:hexBinary("786174616d61616772") eq xs:hexBinary("767479716c6a647663"))