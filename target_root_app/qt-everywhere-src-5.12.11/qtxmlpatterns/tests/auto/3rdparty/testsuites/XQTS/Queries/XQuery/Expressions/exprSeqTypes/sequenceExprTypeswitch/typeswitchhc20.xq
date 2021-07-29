(: Name: typeswitchhq20 :)
(: Description: Simple test for evaluation of atomic value (string) and return clause containing a string operation :)
typeswitch ("A String")
 case $i as xs:decimal
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return <wrap>test failed</wrap> 
 case $i as xs:string
    return fn:string-length($i)
 default
   return <wrap>test failed</wrap>