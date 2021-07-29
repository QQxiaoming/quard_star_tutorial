(: Name: unabbreviatedSyntax-13 :)
(: Description: Evaluate self (self::employee)- Select the context node, if it is an "employee", otherwise return empty sequence :)
(: This test selects an "employee" element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]) 
 return $h/self::employee