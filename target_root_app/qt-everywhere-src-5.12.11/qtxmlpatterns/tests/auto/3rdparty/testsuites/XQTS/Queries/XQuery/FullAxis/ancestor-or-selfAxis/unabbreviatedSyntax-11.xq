(: Name: unabbreviatedSyntax-11 :)
(: Description: Evaluate selecting an ancestor or self (ancestor-or-self::employee)- Select the "employee" ancestors of the context node and if the :)
(: context is "employee" select it as well.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]/hours) 
 return $h/ancestor-or-self::employee