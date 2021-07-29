(: Name: ForExprType029 :)
(: Description: FLWOR expressions with type declaration. Value subtype promotion: Numeric based types -> decimal :)

for $test as xs:decimal in ( xs:integer("100"), xs:short("1"), xs:int("10000"), xs:nonPositiveInteger("-10"), xs:unsignedShort("100"), xs:positiveInteger("100"), xs:nonNegativeInteger("100") )
return $test