(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

declare function local:partners($company as xs:string) as element()*
{
    let $c := $input-context2//company[name = $company]
    return $c//partner
};

for $item in $input-context1//news_item,
    $c in $input-context2//company
let $name := string(exactly-one($c/name)),$partners := local:partners($name)
where contains(string($item), $name)
  and (some $p in $partners satisfies
    contains(string($item), $p) and $item/news_agent != $c/name)
return
    $item 
