(: Name: ForExprType046 :)
(: Description: FLWOR expressions with type declaration. Attempt to promote xs:decimal | xs:double | xs:integer - no common subtype :)

for $test as xs:double in ( 1, 1e2, 0.3 )
return $test