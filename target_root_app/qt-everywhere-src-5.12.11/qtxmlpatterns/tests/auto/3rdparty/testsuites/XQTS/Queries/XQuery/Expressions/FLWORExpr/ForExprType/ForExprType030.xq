(: Name: ForExprType030 :)
(: Description: FLWOR expressions with type declaration. Value subtype promotion: String based types -> normalizedString :)

for $test as xs:normalizedString in ( xs:language("en-US"), xs:NCName("foobar"), xs:NMTOKEN("token1"), xs:ENTITY("entity1") )
return $test