(:*******************************************************:)
(: Test: K2-Axes-81                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Ensure that an element name test after an attribute test matches. :)
(:*******************************************************:)
declare construction strip;

<a>
    <b id="person0">
        <c/>
    </b>
</a>/*[attribute::id = "person0"]/c
