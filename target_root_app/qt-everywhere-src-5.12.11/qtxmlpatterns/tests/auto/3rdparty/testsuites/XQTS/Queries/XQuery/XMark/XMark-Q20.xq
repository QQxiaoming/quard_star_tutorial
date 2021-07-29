(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Group customers by their income and output the cardinality of each group. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q20>
  {
    let $auction := $input-context return
    <result>
      <preferred>
        {count($auction/site/people/person/profile[@income >= 100000.0])}
      </preferred>
      <standard>
        {
          count(
            $auction/site/people/person/
             profile[@income < 100000.0 and @income >= 30000.0]
          )
        }
      </standard>
      <challenge>
        {count($auction/site/people/person/profile[@income < 30000.0])}
      </challenge>
      <na>
        {
          count(
            for $p in $auction/site/people/person
            where empty($p/profile/@income)
            return $p
          )
        }
      </na>
    </result>
  }
</XMark-result-Q20>
