(: Name: fn-doc-33 :)
(: Description: A moderately complex query. :)
declare namespace s = "http://www.w3.org/2000/svg";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare variable $root := doc($input-context)/root/QObject;
declare variable $metaObjects := $root/preceding-sibling::metaObjects/metaObject;
declare function local:recurseMetaObject($object as element(metaObject), $count as xs:integer)
{
    <s:text x ="{10 * $count}" y="10"
font-family="Verdana" font-size="55" fill="blue" >{string($object/@className)}</s:text>
    (:,
    local:recurseMetaObject($metaObjects[@superClass  = $object/@className], $count + 1)
:)
};

declare function local:drawDiagram($object as element(QObject)) as element(s:g)
{
    <s:g>
        {
            (: Output an xml:id attribute on each group element, if we have a name. :)
            if(string($object/@objectName))
            then attribute xml:id {$object/@objectName}
            else (),

            $metaObjects[@className = $object/@className]/local:recurseMetaObject(., 1)
        }
    </s:g>
};

<s:svg>

    {
     <s:rect x="1" y="1" width="500" height="300"
        fill="none" stroke="blue" stroke-width="2"/>,
     <s:rect x="400" y="100" width="400" height="200"
        fill="yellow" stroke="navy" stroke-width="10"  />,
        for $object in $root//QObject
        for $i in local:drawDiagram($object)
        stable order by string($i)
        return $i
    }

</s:svg>
