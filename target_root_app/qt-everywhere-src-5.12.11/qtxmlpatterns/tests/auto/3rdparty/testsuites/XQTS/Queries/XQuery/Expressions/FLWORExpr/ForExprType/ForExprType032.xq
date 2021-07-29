(: Name: ForExprType032 :)
(: Description: FLWOR expressions with type declaration. Value based subtype promotion: 2nd level derived types :)

for $test as xs:anyAtomicType in ( xs:boolean("true"), xs:base64Binary("abcd"), xs:hexBinary("1234"), xs:float("5.7"), xs:double("7.5"), xs:anyURI("http://foo") )
return $test