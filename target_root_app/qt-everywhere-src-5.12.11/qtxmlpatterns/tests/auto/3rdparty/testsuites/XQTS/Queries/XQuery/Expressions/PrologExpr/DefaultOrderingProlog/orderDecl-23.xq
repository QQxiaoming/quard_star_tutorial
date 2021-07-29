(: Name: orderDecl-23:)
(: Description: Simple ordering mode test.  Mode set to "ordered" in the prolog, and use of the "descendant" axis".:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/works//overtime
    return $x/descendant::day

