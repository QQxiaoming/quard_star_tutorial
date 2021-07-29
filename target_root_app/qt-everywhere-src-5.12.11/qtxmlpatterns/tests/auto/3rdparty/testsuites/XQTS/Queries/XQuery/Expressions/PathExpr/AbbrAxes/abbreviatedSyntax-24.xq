(: Name: abbreviatedSyntax-24 :)
(: Description: Evaluates "employee/(status|overtime)/day".  Selects every day element that has a parent that is either a status or an overime element, that in turn is a child of an employee element that is a child of the context node. Uses "union" Operator:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee/(status union overtime)/day