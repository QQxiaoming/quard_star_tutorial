(:*******************************************************:)
(: Test: K2-Axes-78                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to a set of nodes involving the parent axis. :)
(:*******************************************************:)
<a>
    <b c=""/>
    <d/>
</a>//node()/../count(.)