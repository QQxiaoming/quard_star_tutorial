(: Name: orderDecl-2:)
(: Description: Simple ordering mode test.  Mode set to "ordered".  Use "union" operator.:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $a := <a><b>1</b><c>2</c></a>
return $a/b union $a/c