(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
  {
    for $id in $input-context//xref/@xrefid
    return $input-context//topic[@topicid = $id]
  }
</result> 