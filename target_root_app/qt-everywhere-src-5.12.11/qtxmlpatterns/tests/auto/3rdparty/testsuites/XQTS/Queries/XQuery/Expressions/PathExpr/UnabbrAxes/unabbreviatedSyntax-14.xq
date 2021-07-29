(: Name: unabbreviatedSyntax-14 :)
(: Description: Evaluate self (self::hours)- Select the context node, if it is an "employee", otherwise return empty sequence :)
(: This test selects an empty sequence. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works[1]/employee[1]) 
 return fn:count(($h/self::employee[1000]))