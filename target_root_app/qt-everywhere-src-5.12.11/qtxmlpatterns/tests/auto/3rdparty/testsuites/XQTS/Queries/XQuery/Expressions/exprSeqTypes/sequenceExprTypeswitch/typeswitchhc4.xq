(: Name: typeswitchhq4 :)
(: Description: Simple test for evaluation of atomic value (string) and various data types for case evaluation :)
typeswitch ("A String")
 case $i as xs:decimal
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return <wrap>test failed</wrap> 
 case $i as xs:string
    return <wrap>test passed - "A String" is a string type</wrap>
 default
   return <wrap>test failed</wrap>