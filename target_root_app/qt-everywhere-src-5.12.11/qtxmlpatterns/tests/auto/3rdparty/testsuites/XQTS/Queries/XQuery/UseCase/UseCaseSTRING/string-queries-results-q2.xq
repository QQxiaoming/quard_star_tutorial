(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

declare function local:partners($company as xs:string) as element()*
{
    let $c := $input-context2//company[name = $company]
    return $c//partner
};

let $foobar_partners := local:partners("Foobar Corporation")

for $item in $input-context1//news_item
where
  some $t in $item//title satisfies
    (contains(exactly-one($t/text()), "Foobar Corporation")
    and (some $partner in $foobar_partners satisfies
      contains(exactly-one($t/text()), $partner/text())))
  or (some $par in $item//par satisfies
   (contains(string($par), "Foobar Corporation")
     and (some $partner in $foobar_partners satisfies
        contains(string($par), $partner/text())))) 
return
    <news_item>
        { $item/title }
        { $item/date }
    </news_item> 
