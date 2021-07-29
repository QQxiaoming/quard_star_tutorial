(: Name: unabbreviatedSyntax-12 :)
(: Description: Evaluate selecting an descendant or self (descendant-or-self::employee)- Select the "employee" descendants of the context node and if the :)
(: context is "employee" select it as well.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]) 
 return $h/descendant-or-self::employee