(: insert-start :)
declare variable $input-context external;
(: insert-end :)

let $item := $input-context//item_tuple
  [end_date >= xs:date("1999-03-01") and end_date <= xs:date("1999-03-31")]
return
    <item_count>
      { 
        count($item) 
      }
    </item_count>