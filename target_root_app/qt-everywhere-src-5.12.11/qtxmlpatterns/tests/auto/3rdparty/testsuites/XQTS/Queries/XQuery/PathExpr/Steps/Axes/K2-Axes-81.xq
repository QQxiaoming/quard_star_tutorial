(:*******************************************************:)
(: Test: K2-Axes-81                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that an element name test after an attribute test matches. :)
(:*******************************************************:)
<a>
    <b id="person0">
        <c/>
    </b>
</a>/*[attribute::id eq "person0"]/c