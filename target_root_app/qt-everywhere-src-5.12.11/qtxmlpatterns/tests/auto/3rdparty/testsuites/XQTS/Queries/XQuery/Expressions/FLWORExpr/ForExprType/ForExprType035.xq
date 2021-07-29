(: Name: ForExprType035 :)
(: Description: FLWOR expressions with type declaration. Value based subtype promotion: string and numeric literal values -> anyAtomicType :)

for $test as xs:anyAtomicType in ( 1, "string", 1e2, 0.3 )
return $test