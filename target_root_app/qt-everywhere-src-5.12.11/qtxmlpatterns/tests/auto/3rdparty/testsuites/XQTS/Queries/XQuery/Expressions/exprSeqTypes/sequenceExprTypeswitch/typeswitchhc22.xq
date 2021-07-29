(: Name: typeswitchhq22 :)
(: Description: Simple test for evaluation of atomic value ("1") and return clause containing a subtraction operation :)
typeswitch (1)
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return 5 - 3
 case $i as xs:string
    return <wrap>test failed</wrap>
 default
   return <wrap>test failed</wrap>