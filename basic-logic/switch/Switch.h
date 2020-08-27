/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#include <homegear-node/INode.h>
#include <homegear-node/JsonDecoder.h>
#include <homegear-node/MessageProperty.h>
#include <mutex>
#include <regex>

namespace Switch {

class Switch : public Flows::INode {
 public:
  enum class RuleType {
    tEq,
    tNeq,
    tLt,
    tLte,
    tGt,
    tGte,
    tBtwn,
    tCont,
    tRegex,
    tTrue,
    tFalse,
    tNull,
    tNnull,
    tElse
  };

  Switch(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected);
  ~Switch() override;

  bool init(const Flows::PNodeInfo &info) override;
 private:
  struct Rule {
    RuleType t;
    Flows::PVariable v;
    Flows::VariableType vt;
    Flows::PVariable previousOutput;
    bool previousValue = false;
    bool secondInput = false;
    std::string flowVariable;
    std::string globalVariable;
    std::string envVariable;
    bool ignoreCase = false;
    Flows::PVariable v2;
    Flows::VariableType v2t;
    bool previousValue2 = false;
    bool secondInput2 = false;
    std::string flowVariable2;
    std::string globalVariable2;
    std::string envVariable2;
    std::regex regex;
  };

  typedef std::string Operator;

  Flows::MessageProperty _property;
  Flows::PVariable _previousInputValue;
  Flows::PVariable _previousInputValue2;
  std::vector<Rule> _rules;
  bool _changesOnly = false;
  bool _outputTrue = false;
  bool _outputFalse = false;
  bool _checkAll = true;
  bool _staticOnly = false;

  Flows::PVariable _value;
  std::string _payloadType;

  RuleType getRuleTypeFromString(std::string &t);
  Flows::VariableType getValueTypeFromString(std::string &vt);
  static void convertType(Flows::PVariable &value, Flows::VariableType vt);
  bool isTrue(Flows::PVariable &value);
  bool match(const Flows::PNodeInfo &nodeInfo, Rule &rule, Flows::PVariable &value);
  void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
};

}

#endif
