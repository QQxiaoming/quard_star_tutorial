(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $item in $input-context//news_item
where contains(string(exactly-one($item/content)), "Gorilla Corporation")
return
    <item_summary>
        { concat($item/title,". ") }
        { concat($item/date,". ") }
        { string(($item//par)[1]) }
    </item_summary> 
