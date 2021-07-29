(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Return the name of the person with ID `person0'. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q1>
  {
    let $auction := $input-context return
    for $b in $auction/site/people/person[@id = "person0"] return $b/name/text()
  }
</XMark-result-Q1>
