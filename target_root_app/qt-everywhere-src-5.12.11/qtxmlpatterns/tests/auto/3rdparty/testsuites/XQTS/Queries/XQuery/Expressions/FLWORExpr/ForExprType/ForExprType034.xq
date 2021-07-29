(: Name: ForExprType034 :)
(: Description: FLWOR expressions with type declaration. Value based subtype promotion: numeric literal values -> double :)

for $test as xs:decimal in ( 1, 0.3 )
return $test