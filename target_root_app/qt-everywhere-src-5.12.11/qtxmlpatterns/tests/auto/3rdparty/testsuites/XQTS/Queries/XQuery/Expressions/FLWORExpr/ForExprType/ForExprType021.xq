(: Name: ForExprType021 :)
(: Description: Match a 'document' type :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $doc as document-node() in $input-context
return count( $doc )