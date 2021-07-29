(: Name: ForExprType047 :)
(: Description: FLWOR expressions with type declaration. Test matching no atomic simple types :)

for $test as xs:NMTOKENS in ( xs:NMTOKEN("ab"), xs:NMTOKEN("cd") )
return $test