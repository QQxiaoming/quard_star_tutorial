(: Name: ExpandedQNameConstructFunc017 :)
(: Description: Test function fn:QName. Select local-name part from source document :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName( "http://www.example.com/example", string(($input-context//FolderName)[2]) )}{ "test" }
