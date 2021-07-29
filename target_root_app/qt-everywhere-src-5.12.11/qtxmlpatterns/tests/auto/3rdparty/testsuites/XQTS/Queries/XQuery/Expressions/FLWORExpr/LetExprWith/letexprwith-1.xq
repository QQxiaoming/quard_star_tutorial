(: Name: letexprwith-1 :)
(: Description: Evaluation of let clause bound to variable used in expression that raises error. :)

let $salary as xs:decimal :=  "cat"
return $salary * 2