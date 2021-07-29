(: Name: ForExprType033 :)
(: Description: FLWOR expressions with type declaration. Value based subtype promotion: Types from all levels :)

for $test as xs:anyAtomicType in ( xs:unsignedByte("10"), xs:short("20"), xs:positiveInteger("30"), xs:nonPositiveInteger("-40"), xs:decimal("5.5"), xs:ENTITY("entity1"), xs:NCName("foobar"), xs:language("en-US"), xs:string("foobar"), xs:hexBinary("ffff"), xs:gYear("1999") )
return $test