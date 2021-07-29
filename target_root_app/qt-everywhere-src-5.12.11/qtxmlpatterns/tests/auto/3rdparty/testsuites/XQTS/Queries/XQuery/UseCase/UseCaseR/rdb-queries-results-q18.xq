(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
declare variable $input-context3 external;
(: insert-end :)
<result>
  {
    for $u in $input-context2//user_tuple
    order by $u/name
    return
        <user>
            { $u/name }
            {
                for $b in distinct-values($input-context3//bid_tuple
                                             [userid = $u/userid]/itemno)
                for $i in $input-context1//item_tuple[itemno = $b]
                let $descr := $i/description/text()
                order by exactly-one($descr)
                return
                    <bid_on_item>{ $descr }</bid_on_item>
            }
        </user>
  }
</result>
