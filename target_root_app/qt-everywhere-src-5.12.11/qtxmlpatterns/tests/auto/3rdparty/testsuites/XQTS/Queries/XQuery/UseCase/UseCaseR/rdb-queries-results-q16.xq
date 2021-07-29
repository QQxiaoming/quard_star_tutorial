(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)
<result>
  {
    for $u in $input-context1//user_tuple
    let $b := $input-context2//bid_tuple[userid = $u/userid]
    order by exactly-one($u/userid)
    return
        <user>
            { $u/userid }
            { $u/name }
            {
                if (empty($b))
                  then <status>inactive</status>
                  else <status>active</status>
            }
        </user>
  }
</result>
