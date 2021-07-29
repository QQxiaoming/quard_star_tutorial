(: Name: predicates-31:)
(: Description: Evaluation of predicates reordering by an implementation :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/works/employee[fn:position() mod 2 eq 1][fn:position() lt 5]
return 
 (fn:data($x/empnum), fn:data($x/pnum))