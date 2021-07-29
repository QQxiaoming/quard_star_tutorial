(: Name: orderDecl-22:)
(: Description: Simple ordering mode test.  Mode set to "ordered" in the prolog, and use of the "following" axis".:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1/works//day[1]
    return $x/following::day
