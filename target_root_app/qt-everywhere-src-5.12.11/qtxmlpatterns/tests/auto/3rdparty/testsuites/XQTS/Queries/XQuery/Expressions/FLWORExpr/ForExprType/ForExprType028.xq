(: Name: ForExprType028 :)
(: Description: FLWOR expressions with type declaration. Value subtype promotion: Numeric based types -> integer :)

for $test as xs:integer in ( xs:byte("1"), xs:long("10000"), xs:negativeInteger("-10"), xs:short("100"), xs:positiveInteger("100"), xs:nonNegativeInteger("100") )
return $test
