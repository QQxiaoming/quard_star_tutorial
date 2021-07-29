(: Name: orderDecl-20:)
(: Description: Simple ordering mode test.  Mode set to "ordered" in the prolog, and use of the "child" axis".:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/works//overtime
    return $x/child::day

