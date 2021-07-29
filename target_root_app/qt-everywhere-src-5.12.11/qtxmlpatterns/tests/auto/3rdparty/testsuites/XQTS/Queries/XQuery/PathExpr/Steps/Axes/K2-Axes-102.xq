(:*******************************************************:)
(: Test: K2-Axes-102                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Execute a query in several different ways.   :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $a in $input-context1 return
    for $b in $a/child::site return
        for $c in $b/child::people return
            for $d in $c/child::person return
                if ((some $id in $d/attribute::id
                     satisfies typeswitch ($id)
                               case $n as node() return $id = "person0"
                               default $d return ()))
                then $d/child::name
                else (),

for $b in $input-context1/site/people/person
where $b/@id="person0"
return $b/name,

$input-context1/site/people/person[@id eq "person0"]/name