(:*******************************************************:)
(:Test: minutes-from-dateTime-19                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z")) ge fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z"))
