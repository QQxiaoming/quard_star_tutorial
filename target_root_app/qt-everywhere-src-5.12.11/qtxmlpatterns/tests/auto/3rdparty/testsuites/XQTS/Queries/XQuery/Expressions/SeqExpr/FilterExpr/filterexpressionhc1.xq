(: Name: filterexpressionhc1 :)
(: Description: Filter Expression. Simple Filter expression involving numeric data and (gt operator0):)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[xs:integer(hours[1]) gt 20])