(: Name: typeswitchhq17 :)
(: Description: Simple test for evaluation of atomic value (integer) and return contains integer expression :)

typeswitch (5)
 case $i as xs:integer
   return xs:integer(1 + 1)
 case $i as xs:date
   return <wrap>test failed</wrap> 
 case $i as xs:time
   return <wrap>test failed</wrap>
 case $i as xs:string
    return <wrap>test failed</wrap>
 default
    return <wrap>test failed</wrap>