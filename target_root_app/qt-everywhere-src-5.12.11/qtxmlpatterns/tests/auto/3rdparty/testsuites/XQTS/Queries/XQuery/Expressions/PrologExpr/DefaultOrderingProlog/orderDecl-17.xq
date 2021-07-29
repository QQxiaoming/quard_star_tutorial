(: Name: orderDecl-17:)
(: Description: Simple ordering mode test.  Mode set to "unordered" in the prolog, but overridden by an "ordered" expression :)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered
 { for $x in $input-context1/works//day
    return $x
}