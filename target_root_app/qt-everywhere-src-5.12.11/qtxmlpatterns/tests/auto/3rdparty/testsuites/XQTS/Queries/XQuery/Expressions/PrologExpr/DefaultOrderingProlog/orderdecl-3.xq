(: Name: orderDecl-3:)
(: Description: Simple ordering mode test.  Mode set to "unordered".  Use "union" operator.:)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(<a>1</a>) union (<b>2</b>)