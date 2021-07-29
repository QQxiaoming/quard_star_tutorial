(:*******************************************************:)
(: Test: K2-Axes-86                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A significant set of path expressions with different combinations of source values, axes and node tests, that always evaluate to the empty sequence, no matter what document structure that is being walked. The list is not guaranteed to be exhaustive. Some implementations detect this and rewrite away such combinations or warn the user about such constructs. Since static typing implementations may infer these expressions to the empty sequence, they may raise XPST0005. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
declare variable $root as document-node() := $input-context1 treat as document-node();
<empty>
{
    (:  attribute axis, and all other axes that inference can assert might produce attributes.
        These are placed first, so we don't raise XQTY0024. :)
    $root/foo/child::attribute(foo),
    $root//element()/self::attribute(),
    $root//text()/self::attribute(),
    $root/self::attribute(),
    $root/foo/descendant::attribute(foo),
    $root//processing-instruction()/self::attribute(),

    $root/attribute(foo),
    $root/attribute(),
    $root/attribute::attribute(),
    $root//parent::attribute(),
    $root//document-node()/self::attribute(),
    $root/@*,
    $root/*/@*[self::*],
    $root//comment()/descendant-or-self::attribute(),
    $root//processing-instruction()/descendant-or-self::attribute(),
    $root//text()/descendant-or-self::attribute(),
    $root//document-node()/descendant-or-self::attribute(),
    $root//parent::attribute(),
    $root//ancestor::attribute(),
    $root//ancestor-or-self::attribute(),
    $root/self::attribute(),

    $root//attribute()/child::node(),
    $root//attribute::text(),
    $root//attribute::comment(),
    $root//attribute::processing-instruction(),
    $root//attribute::document-node(),
    $root//attribute::document-node(element(foo)),
    $root//attribute()/self::text(),
    $root//attribute()/self::processing-instruction(),
    $root//attribute()/self::element(),
    $root//attribute()/self::document-node(),
    $root//attribute()/self::comment(),
    $root//*/attribute()//node(),
    $root//*/attribute()/descendant::node(),
    $root//attribute::element(),


    $root//comment()/child::node(),
    $root//processing-instruction()/child::node(),
    $root//text()/child::node(),

    (:--------- descendant:: -------- :)
    $root//attribute()/descendant::node(),
    $root//comment()/descendant::node(),
    $root//text()/descendant::node(),
    $root//processing-instruction()/descendant::node(),

    (:--------- descendant-or-self:: -------- :)
    $root//comment()/descendant-or-self::text(),
    $root//comment()/descendant-or-self::processing-instruction(),
    $root//comment()/descendant-or-self::element(),
    $root//comment()/descendant-or-self::document-node(),

    $root//processing-instruction()/descendant-or-self::text(),
    $root//processing-instruction()/descendant-or-self::comment(),
    $root//processing-instruction()/descendant-or-self::element(),
    $root//processing-instruction()/descendant-or-self::document-node(),

    $root//text()/descendant-or-self::processing-instruction(),
    $root//text()/descendant-or-self::comment(),
    $root//text()/descendant-or-self::element(),
    $root//text()/descendant-or-self::document-node(),

    $root//attribute()/descendant-or-self::processing-instruction(),
    $root//attribute()/descendant-or-self::text(),
    $root//attribute()/descendant-or-self::comment(),
    $root//attribute()/descendant-or-self::element(),
    $root//attribute()/descendant-or-self::document-node(),

    (:--------- parent:: -------- :)
    $root/..,
    $root//parent::comment(),
    $root//parent::processing-instruction(),
    $root//parent::text(),

    (:--------- ancestor:: -------- :)
    $root//ancestor::comment(),
    $root//ancestor::processing-instruction(),
    $root//ancestor::text(),

    (:--------- ancestor-or-self:: -------- :)

    (:--------- self:: -------- :)
    $root/self::comment(),
    $root/self::processing-instruction(),
    $root/self::text(),
    $root/self::element(),

    $root//element()/self::text(),
    $root//element()/self::processing-instruction(),
    $root//element()/self::document-node(),
    $root//element()/self::comment(),

    $root//processing-instruction()/self::text(),
    $root//processing-instruction()/self::element(),
    $root//processing-instruction()/self::document-node(),
    $root//processing-instruction()/self::comment(),

    $root//text()/self::processing-instruction(),
    $root//text()/self::element(),
    $root//text()/self::document-node(),
    $root//text()/self::comment(),

    $root//document-node()/self::processing-instruction(),
    $root//document-node()/self::element(),
    $root//document-node()/self::text(),
    $root//document-node()/self::comment()
}
</empty>,
exists($root//*/attribute()/descendant-or-self::node())