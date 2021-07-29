(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $b in $input-context//book
let $e := $b/*[contains(string(.), "Suciu") 
               and ends-with(local-name(.), "or")]
where exists($e)
return
    <book>
        { $b/title }
        { $e }
    </book> 