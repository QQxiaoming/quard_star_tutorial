(:*******************************************************:)
(:Test: day-from-date-13                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:day-from-date(xs:date("1970-01-02Z")) mod fn:day-from-date(xs:date("1970-01-01Z"))
