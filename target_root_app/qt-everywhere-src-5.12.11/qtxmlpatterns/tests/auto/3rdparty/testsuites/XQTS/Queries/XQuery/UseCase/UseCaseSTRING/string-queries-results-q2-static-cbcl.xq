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
  some $t in $item//title/string() satisfies
    (
      contains($t, "Foobar Corporation")
      and (
          some $partner in $foobar_partners satisfies
          contains($t, string($partner))
      )
      or (
          some $par in $item//par/string() satisfies
          (
             contains($par, "Foobar Corporation")
             and (
                 some $partner in $foobar_partners satisfies
                 contains($par, string($partner))
             ) 
          )
      )
  )
return
    <news_item>
        { $item/title }
        { $item/date }
    </news_item> 
