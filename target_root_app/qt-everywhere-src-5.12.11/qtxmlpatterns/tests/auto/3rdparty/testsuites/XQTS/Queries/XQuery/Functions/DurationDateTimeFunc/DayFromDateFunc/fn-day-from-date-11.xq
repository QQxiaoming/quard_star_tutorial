(:*******************************************************:)
(:Test: day-from-date-11                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:day-from-date(xs:date("1970-01-02Z")) div fn:day-from-date(xs:date("1970-01-01Z"))
