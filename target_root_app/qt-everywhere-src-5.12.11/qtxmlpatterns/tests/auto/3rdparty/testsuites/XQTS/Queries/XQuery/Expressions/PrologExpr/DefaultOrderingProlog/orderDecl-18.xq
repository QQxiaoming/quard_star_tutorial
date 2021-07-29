(: Name: orderDecl-18:)
(: Description: Simple ordering mode test.  Mode set to "ordered" in the prolog, but overridden by an "unordered" expression :)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

unordered
 { for $x in $input-context1/works//day
    return $x
}